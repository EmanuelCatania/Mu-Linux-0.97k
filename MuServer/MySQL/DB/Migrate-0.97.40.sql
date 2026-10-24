-- Migración a 0.97.40: nombres UTF-8 más largos.
--
-- Requiere la base ya en utf8mb4 (Migrate-utf8mb4.sql). Los nombres de
-- personaje admiten 10 grapheme clusters (hasta 40 bytes), los de guild 8
-- (hasta 32) y el aviso del guild hasta 200 bytes. VARCHAR cuenta caracteres,
-- así que estos largos alcanzan para cualquier nombre que acepte el server.
--
-- Después de esta migración hay que volver a ejecutar Procedures.sql, que
-- recrea los procedimientos con los parámetros nuevos.

USE MuOnline97;

ALTER TABLE `AccountCharacter`
	MODIFY `GameID1` VARCHAR(40) NULL,
	MODIFY `GameID2` VARCHAR(40) NULL,
	MODIFY `GameID3` VARCHAR(40) NULL,
	MODIFY `GameID4` VARCHAR(40) NULL,
	MODIFY `GameID5` VARCHAR(40) NULL,
	MODIFY `GameIDC` VARCHAR(40) NULL;

ALTER TABLE `Character` MODIFY `Name` VARCHAR(40) NOT NULL;

ALTER TABLE `Guild`
	MODIFY `G_Name` VARCHAR(32) NOT NULL,
	MODIFY `G_Master` VARCHAR(40) NULL,
	MODIFY `G_Notice` VARCHAR(200) NULL;

ALTER TABLE `GuildMember`
	MODIFY `Name` VARCHAR(40) NOT NULL,
	MODIFY `G_Name` VARCHAR(32) NOT NULL;

ALTER TABLE `OptionData` MODIFY `Name` VARCHAR(40) NOT NULL;
ALTER TABLE `RankingBloodCastle` MODIFY `Name` VARCHAR(40) NOT NULL;
ALTER TABLE `RankingDevilSquare` MODIFY `Name` VARCHAR(40) NOT NULL;
ALTER TABLE `ResetInfo` MODIFY `Name` VARCHAR(40) NOT NULL;

SELECT TABLE_NAME, COLUMN_NAME, CHARACTER_MAXIMUM_LENGTH, COLLATION_NAME
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = 'MuOnline97'
  AND COLUMN_NAME IN ('GameID1', 'GameID2', 'GameID3', 'GameID4', 'GameID5', 'GameIDC', 'Name', 'G_Name', 'G_Master', 'G_Notice')
ORDER BY TABLE_NAME, COLUMN_NAME;
