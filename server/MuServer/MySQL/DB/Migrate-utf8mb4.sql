-- ----------------------------------------------------------------------------
-- Migración de MuOnline97 a utf8mb4 (0.97.40)
-- ----------------------------------------------------------------------------
-- Antes de ejecutarlo: backup completo (ver docs/utf8mb4-migracion.md).
-- Después de ejecutarlo: Procedures.sql, para recrear los procedimientos con
-- el charset nuevo.
--
-- Convierte la base y todas sus tablas a utf8mb4 con la collation
-- utf8mb4_0900_as_ci (distingue acentos y no mayúsculas). Los datos latin1 de
-- MySQL son Windows-1252, así que los acentos se conservan.
--
-- Primero verifica que ningún nombre (personaje, cuenta, guild) se vuelva
-- igual a otro con la collation nueva; si pasa, se detiene sin tocar nada y
-- lista los nombres en conflicto.
USE `MuOnline97`;

DROP PROCEDURE IF EXISTS `MU_UTF8MB4_CHECK`;
DROP PROCEDURE IF EXISTS `MU_UTF8MB4_CONVERT`;

DELIMITER $$

CREATE PROCEDURE `MU_UTF8MB4_CHECK`()
BEGIN
	DROP TEMPORARY TABLE IF EXISTS `mu_utf8mb4_conflicts`;
	CREATE TEMPORARY TABLE `mu_utf8mb4_conflicts` (
		`TableName` VARCHAR(64) NOT NULL,
		`Value` VARCHAR(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
		`Count` INT NOT NULL
	);

	INSERT INTO `mu_utf8mb4_conflicts`
	SELECT 'Character', MIN(CONVERT(`Name` USING utf8mb4)), COUNT(*) FROM `Character`
	GROUP BY CONVERT(`Name` USING utf8mb4) COLLATE utf8mb4_0900_as_ci HAVING COUNT(*) > 1;

	INSERT INTO `mu_utf8mb4_conflicts`
	SELECT 'Guild', MIN(CONVERT(`G_Name` USING utf8mb4)), COUNT(*) FROM `Guild`
	GROUP BY CONVERT(`G_Name` USING utf8mb4) COLLATE utf8mb4_0900_as_ci HAVING COUNT(*) > 1;

	INSERT INTO `mu_utf8mb4_conflicts`
	SELECT 'MEMB_INFO', MIN(CONVERT(`memb___id` USING utf8mb4)), COUNT(*) FROM `MEMB_INFO`
	GROUP BY CONVERT(`memb___id` USING utf8mb4) COLLATE utf8mb4_0900_as_ci HAVING COUNT(*) > 1;

	IF (SELECT COUNT(*) FROM `mu_utf8mb4_conflicts`) > 0 THEN
		SELECT * FROM `mu_utf8mb4_conflicts`;
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Hay nombres que chocan con utf8mb4_0900_as_ci; no se convirtio nada.';
	END IF;
END $$

CREATE PROCEDURE `MU_UTF8MB4_CONVERT`()
BEGIN
	DECLARE done INT DEFAULT 0;
	DECLARE tableName VARCHAR(64);
	DECLARE pending CURSOR FOR SELECT `Name` FROM `mu_utf8mb4_tables`;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;

	-- La lista se toma antes de empezar: el ALTER cambia information_schema.
	DROP TEMPORARY TABLE IF EXISTS `mu_utf8mb4_tables`;
	CREATE TEMPORARY TABLE `mu_utf8mb4_tables` (`Name` VARCHAR(64) NOT NULL);
	INSERT INTO `mu_utf8mb4_tables`
	SELECT TABLE_NAME FROM information_schema.TABLES
	WHERE TABLE_SCHEMA = DATABASE() AND TABLE_TYPE = 'BASE TABLE'
		AND TABLE_COLLATION <> 'utf8mb4_0900_as_ci';

	OPEN pending;
	convert_loop: LOOP
		FETCH pending INTO tableName;
		IF done THEN
			LEAVE convert_loop;
		END IF;
		SET @mu_sql = CONCAT('ALTER TABLE `', tableName, '` CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci');
		PREPARE mu_statement FROM @mu_sql;
		EXECUTE mu_statement;
		DEALLOCATE PREPARE mu_statement;
	END LOOP;
	CLOSE pending;
END $$

DELIMITER ;

CALL `MU_UTF8MB4_CHECK`();

ALTER DATABASE `MuOnline97` CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_as_ci;

CALL `MU_UTF8MB4_CONVERT`();

DROP PROCEDURE `MU_UTF8MB4_CHECK`;
DROP PROCEDURE `MU_UTF8MB4_CONVERT`;

SELECT TABLE_NAME, TABLE_COLLATION FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE() ORDER BY TABLE_NAME;
